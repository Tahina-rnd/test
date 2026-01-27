import {
  Injectable,
  UnauthorizedException,
  ConflictException,
  NotFoundException,
} from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { InjectRepository } from '@nestjs/typeorm';
import { Repository } from 'typeorm';
import * as bcrypt from 'bcrypt';
import { User } from '../users/entities/user.entity';
import { RegisterDto } from './dto/register.dto';
import { LoginDto } from './dto/login.dto';
import { UserRegisterResponse, UserResponse } from './dto/user.dto';
import { ProofDto } from './dto/proof.dto';
import { UserStatus, UserRole } from '@enums/user.enum';

@Injectable()
export class AuthService {
  constructor(
    @InjectRepository(User)
    private userRepository: Repository<User>,
    private jwtService: JwtService,
  ) {}

  /**
   * Register a new user.
   *
   * @param registerDto - The user data to register.
   *
   * @returns A promise that resolves to an object containing the registered user, an access token and a refresh token.
   *
   * @throws ConflictException - If the email is already used.
   */
  async register(registerDto: RegisterDto): Promise<{
    user: UserRegisterResponse;
    accessToken: string;
    refreshToken: string;
  }> {
    const existingUser = await this.userRepository.findOne({
      where: { email: registerDto.email },
    });

    if (existingUser) {
      throw new ConflictException('Email already used');
    }

    const hashedPassword = await bcrypt.hash(registerDto.password, 10);

    const user = this.userRepository.create({
      ...registerDto,
      password: hashedPassword,
      role: UserRole.STUDENT,
      status: UserStatus.INACTIVE,
      isVerified: false,
    });

    await this.userRepository.save(user);
    const { accessToken, refreshToken } = this.generateTokens(user);

    return { user, accessToken, refreshToken };
  }

  /**
   * Login an existing user.
   *
   * @param loginDto - The user data to login.
   *
   * @returns A promise that resolves to an object containing the logged in user, an access token and a refresh token.
   *
   * @throws UnauthorizedException - If the email or password is incorrect, or if the account is not active.
   */
  async login(loginDto: LoginDto): Promise<{
    user: UserResponse;
    accessToken: string;
    refreshToken: string;
  }> {
    const user = await this.userRepository.findOne({
      where: { email: loginDto.email },
    });

    if (!user || !(await bcrypt.compare(loginDto.password, user.password))) {
      throw new UnauthorizedException('Email or password is incorrect');
    }

    switch (user.status) {
      case UserStatus.ACTIVE:
        break;
      case UserStatus.INACTIVE:
        throw new UnauthorizedException(
          "Veuillez attendre que l'admin actif valide votre compte",
        );
      case UserStatus.BANNED:
        throw new UnauthorizedException('Account banned');
      default:
        throw new UnauthorizedException('Account is inactive or banned');
    }

    const { accessToken, refreshToken } = this.generateTokens(user);

    return { user, accessToken, refreshToken };
  }

  /**
   * Refresh an access token with a valid refresh token.
   *
   * @param refreshToken - The refresh token to use for refreshing the access token.
   *
   * @returns A promise that resolves to an object containing the new access token and the new refresh token.
   *
   * @throws UnauthorizedException - If the refresh token is invalid or expired.
   */
  async refresh(
    refreshToken: string,
  ): Promise<{ accessToken: string; refreshToken: string }> {
    try {
      const payload = this.jwtService.verify(refreshToken, {
        secret: process.env.JWT_REFRESH_SECRET || 'refresh-secret-key',
      });

      const user = await this.userRepository.findOne({
        where: { id: payload.sub },
      });

      if (!user || user.status !== UserStatus.ACTIVE) {
        throw new UnauthorizedException('Invalid token');
      }

      const tokens = this.generateTokens(user);
      return tokens;
    } catch (error) {
      throw new UnauthorizedException('Invalid token');
    }
  }

  /**
   * Validates a user by id.
   *
   * @param userId - The id of the user to validate.
   *
   * @returns A promise that resolves to the validated user.
   *
   * @throws NotFoundException - If the user with the given id is not found.
   */
  async validateUser(userId: string): Promise<UserResponse> {
    const user = await this.userRepository.findOne({ where: { id: userId } });
    if (!user) {
      throw new NotFoundException(`User with id ${userId} not found`);
    }
    return user;
  }

  /**
   * Send a proof of the user's student card.
   *
   * @param userId - The id of the user to send the proof for.
   * @param proofDto - The proof data to send.
   * @param files - The files to upload as proof.
   *
   * @returns A promise that resolves to an object containing the user's CIN number, the paths of the uploaded proof pictures and a success message.
   *
   * @throws NotFoundException - If the user with the given id is not found.
   */
  async sendProof(
    userId: string,
    proofDto: ProofDto,
    files: Express.Multer.File[],
  ) {
    const user = await this.userRepository.findOne({ where: { id: userId } });

    if (!user) {
      throw new NotFoundException(`User not found`);
    }

    const studentCardPaths = files.map(
      (file) => `uploads/proofs/${file.filename}`,
    );

    user.cinNumber = proofDto.cinNumber;
    user.setstudentCardArray(studentCardPaths);

    await this.userRepository.save(user);

    return {
      cinNumber: user.cinNumber,
      studentCard: user.getstudentCardArray(),
      message: 'Proof sent successfully',
    };
  }

  /**
   * Generates an access token and a refresh token for the given user.
   *
   * The access token expires in 15 minutes and the refresh token expires in 7 days.
   *
   * @param user - The user to generate tokens for.
   *
   * @returns An object containing the access token and the refresh token.
   */
  private generateTokens(user: User): {
    accessToken: string;
    refreshToken: string;
  } {
    const payload = {
      sub: user.id,
      email: user.email,
      role: user.role,
    };

    const accessToken = this.jwtService.sign(payload, {
      secret: process.env.JWT_SECRET || 'secret-key',
      expiresIn: '15m',
    });
    const refreshToken = this.jwtService.sign(payload, {
      secret: process.env.JWT_REFRESH_SECRET || 'refresh-secret-key',
      expiresIn: '7d',
    });

    return { accessToken, refreshToken };
  }
}
