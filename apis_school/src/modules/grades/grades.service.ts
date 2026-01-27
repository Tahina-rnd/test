import { Injectable, NotFoundException } from '@nestjs/common';
import { InjectRepository } from '@nestjs/typeorm';
import { Repository } from 'typeorm';
import { Grade, Semester } from './entities/grade.entity';
import { Student } from '../students/entities/student.entity';
import { CreateGradeDto } from './dto/create-grade.dto';
import { UpdateGradeDto } from './dto/update-grade.dto';
import { GradeResponseDto } from './dto/grade-response.dto';

@Injectable()
export class GradesService {
  constructor(
    @InjectRepository(Grade)
    private gradeRepository: Repository<Grade>,
    @InjectRepository(Student)
    private studentRepository: Repository<Student>,
  ) {}

  /**
   * Create a new grade.
   *
   * @param createGradeDto - The grade data to create.
   *
   * @returns A promise that resolves to the created grade.
   *
   * @throws NotFoundException - If the student is not found.
   */
  async create(createGradeDto: CreateGradeDto): Promise<GradeResponseDto> {
    // Check if student exists
    const student = await this.studentRepository.findOne({
      where: { id: createGradeDto.studentId },
    });

    if (!student) {
      throw new NotFoundException(
        `Student with id ${createGradeDto.studentId} not found`,
      );
    }

    const grade = this.gradeRepository.create({
      ...createGradeDto,
      gradeDate: new Date(createGradeDto.gradeDate),
    });

    const savedGrade = await this.gradeRepository.save(grade);
    return this.mapToResponseDto(savedGrade);
  }

  /**
   * Find all grades.
   *
   * @returns A promise that resolves to an array of grades.
   */
  async findAll(): Promise<GradeResponseDto[]> {
    const grades = await this.gradeRepository.find({
      relations: ['student'],
    });
    return grades.map((grade) => this.mapToResponseDto(grade));
  }

  /**
   * Find a grade by id.
   *
   * @param id - The id of the grade to find.
   *
   * @returns A promise that resolves to the grade.
   *
   * @throws NotFoundException - If the grade is not found.
   */
  async findOne(id: string): Promise<GradeResponseDto> {
    const grade = await this.gradeRepository.findOne({
      where: { id },
      relations: ['student'],
    });

    if (!grade) {
      throw new NotFoundException(`Grade with id ${id} not found`);
    }

    return this.mapToResponseDto(grade);
  }

  /**
   * Find grades by student id.
   *
   * @param studentId - The student id to find grades for.
   *
   * @returns A promise that resolves to an array of grades.
   */
  async findByStudentId(studentId: string): Promise<GradeResponseDto[]> {
    const grades = await this.gradeRepository.find({
      where: { studentId },
      relations: ['student'],
      order: { gradeDate: 'DESC' },
    });

    return grades.map((grade) => this.mapToResponseDto(grade));
  }

  /**
   * Find grades by subject code.
   *
   * @param subjectCode - The subject code to find grades for.
   *
   * @returns A promise that resolves to an array of grades.
   */
  async findBySubject(subjectCode: string): Promise<GradeResponseDto[]> {
    const grades = await this.gradeRepository.find({
      where: { subjectCode },
      relations: ['student'],
      order: { gradeDate: 'DESC' },
    });

    return grades.map((grade) => this.mapToResponseDto(grade));
  }

  /**
   * Find grades by academic year and semester.
   *
   * @param academicYear - The academic year.
   * @param semester - The semester.
   *
   * @returns A promise that resolves to an array of grades.
   */
  async findByAcademicPeriod(
    academicYear: string,
    semester: Semester,
  ): Promise<GradeResponseDto[]> {
    const grades = await this.gradeRepository.find({
      where: { academicYear, semester },
      relations: ['student'],
      order: { gradeDate: 'DESC' },
    });

    return grades.map((grade) => this.mapToResponseDto(grade));
  }

  /**
   * Calculate GPA for a student.
   *
   * @param studentId - The student id to calculate GPA for.
   *
   * @returns A promise that resolves to the GPA.
   */
  async calculateGPA(studentId: string): Promise<number> {
    const grades = await this.gradeRepository.find({
      where: { studentId },
    });

    if (grades.length === 0) {
      return 0;
    }

    let totalWeightedScore = 0;
    let totalWeight = 0;

    for (const grade of grades) {
      const percentage = (grade.score / grade.maxScore) * 100;
      totalWeightedScore += percentage * grade.weight;
      totalWeight += grade.weight;
    }

    return totalWeight > 0 ? totalWeightedScore / totalWeight / 25 : 0; // Convert to 4.0 scale
  }

  /**
   * Update a grade.
   *
   * @param id - The id of the grade to update.
   * @param updateGradeDto - The grade data to update.
   *
   * @returns A promise that resolves to the updated grade.
   *
   * @throws NotFoundException - If the grade is not found.
   */
  async update(
    id: string,
    updateGradeDto: UpdateGradeDto,
  ): Promise<GradeResponseDto> {
    const grade = await this.gradeRepository.findOne({
      where: { id },
    });

    if (!grade) {
      throw new NotFoundException(`Grade with id ${id} not found`);
    }

    const updatedGrade = await this.gradeRepository.save({
      ...grade,
      ...updateGradeDto,
      gradeDate:
        updateGradeDto.gradeDate !== undefined
          ? new Date(updateGradeDto.gradeDate)
          : grade.gradeDate,
    });

    return this.mapToResponseDto(updatedGrade);
  }

  /**
   * Remove a grade (soft delete).
   *
   * @param id - The id of the grade to remove.
   *
   * @throws NotFoundException - If the grade is not found.
   */
  async remove(id: string): Promise<void> {
    const grade = await this.gradeRepository.findOne({
      where: { id },
    });

    if (!grade) {
      throw new NotFoundException(`Grade with id ${id} not found`);
    }

    await this.gradeRepository.softDelete(id);
  }

  /**
   * Map a Grade entity to GradeResponseDto.
   *
   * @param grade - The grade entity to map.
   *
   * @returns The mapped GradeResponseDto.
   */
  private mapToResponseDto(grade: Grade): GradeResponseDto {
    return {
      id: grade.id,
      studentId: grade.studentId,
      subjectCode: grade.subjectCode,
      subjectName: grade.subjectName,
      type: grade.type,
      score: grade.score,
      maxScore: grade.maxScore,
      weight: grade.weight,
      letterGrade: grade.letterGrade,
      academicYear: grade.academicYear,
      semester: grade.semester,
      comments: grade.comments,
      gradedBy: grade.gradedBy,
      gradeDate: grade.gradeDate,
      createdAt: grade.createdAt,
      updatedAt: grade.updatedAt,
    };
  }
}
