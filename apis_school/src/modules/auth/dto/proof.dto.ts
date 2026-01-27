import { IsNotEmpty, IsString, Matches } from 'class-validator';
export class ProofDto {
  @IsNotEmpty()
  @IsString()
  @Matches(/^[0-9]{12}$/, { message: 'The CIN number must contain 12 digits' })
  cinNumber: string;
}

export class ProofResponseDto {
  cinNumber: string;
  studentCard: string[];
  message: string;
}
